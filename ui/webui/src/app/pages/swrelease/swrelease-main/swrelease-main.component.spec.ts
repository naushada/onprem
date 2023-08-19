import { ComponentFixture, TestBed } from '@angular/core/testing';

import { SwreleaseMainComponent } from './swrelease-main.component';

describe('SwreleaseMainComponent', () => {
  let component: SwreleaseMainComponent;
  let fixture: ComponentFixture<SwreleaseMainComponent>;

  beforeEach(() => {
    TestBed.configureTestingModule({
      declarations: [SwreleaseMainComponent]
    });
    fixture = TestBed.createComponent(SwreleaseMainComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
