import { ComponentFixture, TestBed } from '@angular/core/testing';

import { CreateSwreleaseComponent } from './create-swrelease.component';

describe('CreateSwreleaseComponent', () => {
  let component: CreateSwreleaseComponent;
  let fixture: ComponentFixture<CreateSwreleaseComponent>;

  beforeEach(() => {
    TestBed.configureTestingModule({
      declarations: [CreateSwreleaseComponent]
    });
    fixture = TestBed.createComponent(CreateSwreleaseComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
