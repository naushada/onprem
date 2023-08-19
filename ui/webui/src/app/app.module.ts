import { NgModule } from '@angular/core';
import { BrowserModule } from '@angular/platform-browser';
import { HttpClientModule } from '@angular/common/http';
import { AppRoutingModule } from './app-routing.module';
import { AppComponent } from './app.component';
import { MainComponent } from './pages/main/main.component';
import { LoginComponent } from './pages/login/login.component';
import { SystemComponent } from './pages/system/system.component';
import { ConfigComponent } from './pages/config/config.component';
import { BrowserAnimationsModule } from '@angular/platform-browser/animations';
import { ClarityModule } from '@clr/angular';
import { CdsIconModule, CdsModule } from '@cds/angular';
import { ClarityIcons, userIcon, homeIcon, vmBugIcon, cogIcon, eyeIcon, barsIcon, newIcon } from '@cds/core/icon';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
ClarityIcons.addIcons(homeIcon, vmBugIcon, cogIcon, eyeIcon, barsIcon,userIcon, newIcon);


@NgModule({
  declarations: [
    AppComponent,
    MainComponent,
    LoginComponent,
    SystemComponent,
    ConfigComponent
  ],
  imports: [
    BrowserModule,
    AppRoutingModule,
    ClarityModule,
    CdsIconModule,
    CdsModule,
    FormsModule,
    ReactiveFormsModule,
    BrowserAnimationsModule,
    HttpClientModule
    
  ],
  providers: [],
  bootstrap: [AppComponent]
})
export class AppModule { }
